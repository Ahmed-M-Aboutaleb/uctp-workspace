from typing import List
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session

from app.infrastructure.database import get_db
from app.api.deps import get_current_admin, get_current_user
from app.models.user import User
from app.models.course import Course
from app.models.people import Instructor, Student
from app.models.facilities import Feature
from app.schemas.course import CourseCreate, CourseResponse, CourseUpdate

router = APIRouter()

@router.post("/", response_model=CourseResponse, status_code=status.HTTP_201_CREATED)
def create_course(
    course_in: CourseCreate, 
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    """Create a new course and link its instructors, students, and required features."""
    course = Course(course_name=course_in.course_name)
    
    # 1. Attach Instructors
    if course_in.instructor_ids:
        instructors = db.query(Instructor).filter(Instructor.instructor_id.in_(course_in.instructor_ids)).all()
        if len(instructors) != len(course_in.instructor_ids):
            raise HTTPException(status_code=404, detail="One or more instructors not found")
        course.instructors = instructors

    # 2. Attach Students
    if course_in.student_ids:
        students = db.query(Student).filter(Student.student_id.in_(course_in.student_ids)).all()
        if len(students) != len(course_in.student_ids):
            raise HTTPException(status_code=404, detail="One or more students not found")
        course.students = students

    # 3. Attach Required Features
    if course_in.feature_ids:
        features = db.query(Feature).filter(Feature.feature_id.in_(course_in.feature_ids)).all()
        if len(features) != len(course_in.feature_ids):
            raise HTTPException(status_code=404, detail="One or more features not found")
        course.features = features

    db.add(course)
    db.commit()
    db.refresh(course)
    return course

@router.get("/", response_model=List[CourseResponse])
def read_courses(
    skip: int = 0, 
    limit: int = 100, 
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    """Retrieve all courses with their nested relationships."""
    return db.query(Course).offset(skip).limit(limit).all()

@router.get("/{course_id}", response_model=CourseResponse)
def read_course(
    course_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    course = db.query(Course).filter(Course.course_id == course_id).first()
    if not course:
        raise HTTPException(status_code=404, detail="Course not found")
    return course

@router.patch("/{course_id}", response_model=CourseResponse)
def update_course(
    course_id: int,
    update_in: CourseUpdate,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    course = db.query(Course).filter(Course.course_id == course_id).first()
    if not course:
        raise HTTPException(status_code=404, detail="Course not found")
    
    if update_in.course_name is not None:
        course.course_name = update_in.course_name
        
    if update_in.instructor_ids is not None:
        instructors = db.query(Instructor).filter(Instructor.instructor_id.in_(update_in.instructor_ids)).all()
        if len(instructors) != len(update_in.instructor_ids):
            raise HTTPException(status_code=404, detail="One or more instructors not found")
        course.instructors = instructors
        
    if update_in.feature_ids is not None:
        features = db.query(Feature).filter(Feature.feature_id.in_(update_in.feature_ids)).all()
        if len(features) != len(update_in.feature_ids):
            raise HTTPException(status_code=404, detail="One or more features not found")
        course.features = features
        
    db.commit()
    db.refresh(course)
    return course

@router.delete("/{course_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_course(
    course_id: int,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    course = db.query(Course).filter(Course.course_id == course_id).first()
    if not course:
        raise HTTPException(status_code=404, detail="Course not found")
        
    db.delete(course)
    db.commit()
    return {
        "message": "Course deleted successfully"
    }