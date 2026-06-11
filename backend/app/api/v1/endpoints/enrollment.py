from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from typing import List

from app.infrastructure.database import get_db
from app.api.deps import get_current_user, get_current_admin
from app.models.user import User, UserRole
from app.models.course import Course
from app.schemas.person import StudentResponse

router = APIRouter()

@router.patch("/{course_id}/toggle", status_code=status.HTTP_200_OK)
def toggle_enrollment(
    course_id: int, 
    is_open: bool, 
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin) # Only the Dean can do this
):
    """Admin endpoint to open or close enrollment for a specific course."""
    course = db.query(Course).filter(Course.course_id == course_id).first()
    if not course:
        raise HTTPException(status_code=404, detail="Course not found")
    
    course.is_enrollment_open = is_open
    db.commit()
    
    status_text = "opened" if is_open else "closed"
    return {"message": f"Enrollment for '{course.course_name}' is now {status_text}."}

@router.post("/{course_id}/join", status_code=status.HTTP_200_OK)
def enroll_student(
    course_id: int, 
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user) # Student must be logged in
):
    """Student endpoint to enroll in a course."""
    # 1. Verify this is actually a student
    if current_user.role != UserRole.STUDENT:
        raise HTTPException(status_code=403, detail="Only students can enroll in courses.")
    
    # 2. Find the course and check if it's open
    course = db.query(Course).filter(Course.course_id == course_id).first()
    if not course:
        raise HTTPException(status_code=404, detail="Course not found")
    if not course.is_enrollment_open:
        raise HTTPException(status_code=400, detail="Enrollment for this course is currently closed.")

    # 3. Get the student's physical profile
    student_profile = current_user.student_profile
    if not student_profile:
        raise HTTPException(
            status_code=404, 
            detail="Your student profile has not been configured yet. Contact the administration."
        )

    # 4. Check if they are already enrolled to prevent duplicates
    if student_profile in course.students:
        raise HTTPException(status_code=400, detail="You are already enrolled in this course.")

    # 5. Enroll the student
    course.students.append(student_profile)
    db.commit()
    
    return {"message": f"Successfully enrolled in {course.course_name}."}

@router.get("/{course_id}/students", response_model=List[StudentResponse])
def read_course_enrollments(
    course_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    """Retrieve all students enrolled in a specific course."""
    course = db.query(Course).filter(Course.course_id == course_id).first()
    if not course:
        raise HTTPException(status_code=404, detail="Course not found")
    return course.students

@router.delete("/{course_id}/leave", status_code=status.HTTP_204_NO_CONTENT)
def unenroll_student(
    course_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    """Student endpoint to unenroll from a course."""
    if current_user.role != UserRole.STUDENT:
        raise HTTPException(status_code=403, detail="Only students can leave courses.")
        
    course = db.query(Course).filter(Course.course_id == course_id).first()
    if not course:
        raise HTTPException(status_code=404, detail="Course not found")
        
    student_profile = current_user.student_profile
    if not student_profile or student_profile not in course.students:
        raise HTTPException(status_code=400, detail="You are not enrolled in this course.")
        
    course.students.remove(student_profile)
    db.commit()
    return {
        "message": "Successfully unenrolled from this course."
    }