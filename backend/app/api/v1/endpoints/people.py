from typing import List, Union
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session

from app.infrastructure.database import get_db
from app.api.deps import get_current_admin, get_current_user
from app.core.security import get_password_hash
from app.models.user import User, UserRole
from app.models.people import Instructor, Student, CollegeAdmin
from app.schemas.person import (
    InstructorCreate, InstructorResponse, InstructorUpdate,
    StudentCreate, StudentResponse, StudentUpdate,
    AdminCreate, AdminResponse, AdminUpdate,
    UpdateProfileRequest
)

router = APIRouter()

# --- INSTRUCTORS ---

@router.post("/instructors", response_model=InstructorResponse, status_code=status.HTTP_201_CREATED)
def create_instructor(
    instructor_in: InstructorCreate, 
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    """Add a new instructor and provision their login account."""
    # 1. Check if email already exists
    if db.query(User).filter(User.email == instructor_in.email).first():
        raise HTTPException(status_code=400, detail="Email already registered in the system.")

    # 2. Create the Auth User
    new_user = User(
        email=instructor_in.email,
        hashed_password=get_password_hash(instructor_in.password),
        role=UserRole.INSTRUCTOR
    )
    db.add(new_user)
    db.flush() # Pushes to DB to generate the ID, but doesn't commit the transaction yet

    # 3. Create the Profile and link the User ID
    instructor = Instructor(
        name=instructor_in.name,
        user_id=new_user.id
    )
    db.add(instructor)
    db.commit()
    db.refresh(instructor)
    return instructor

@router.get("/instructors", response_model=List[InstructorResponse])
def read_instructors(
    skip: int = 0, 
    limit: int = 100, 
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    """Retrieve all instructors."""
    return db.query(Instructor).offset(skip).limit(limit).all()

@router.get("/instructors/{instructor_id}", response_model=InstructorResponse)
def read_instructor(
    instructor_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    instructor = db.query(Instructor).filter(Instructor.instructor_id == instructor_id).first()
    if not instructor:
        raise HTTPException(status_code=404, detail="Instructor not found")
    return instructor

@router.patch("/instructors/{instructor_id}", response_model=InstructorResponse)
def update_instructor(
    instructor_id: int,
    update_in: InstructorUpdate,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    instructor = db.query(Instructor).filter(Instructor.instructor_id == instructor_id).first()
    if not instructor:
        raise HTTPException(status_code=404, detail="Instructor not found")
    
    if update_in.name is not None:
        instructor.name = update_in.name
    
    db.commit()
    db.refresh(instructor)
    return instructor

@router.delete("/instructors/{instructor_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_instructor(
    instructor_id: int,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    instructor = db.query(Instructor).filter(Instructor.instructor_id == instructor_id).first()
    if not instructor:
        raise HTTPException(status_code=404, detail="Instructor not found")
    
    if instructor.user_id:
        user = db.query(User).filter(User.id == instructor.user_id).first()
        if user:
            db.delete(user)
    
    db.delete(instructor)
    db.commit()

# --- STUDENTS ---

@router.post("/students", response_model=StudentResponse, status_code=status.HTTP_201_CREATED)
def create_student(
    student_in: StudentCreate, 
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    """Add a new student and provision their login account."""
    # 1. Check if email already exists
    if db.query(User).filter(User.email == student_in.email).first():
        raise HTTPException(status_code=400, detail="Email already registered in the system.")

    # 2. Create the Auth User
    new_user = User(
        email=student_in.email,
        hashed_password=get_password_hash(student_in.password),
        role=UserRole.STUDENT
    )
    db.add(new_user)
    db.flush()

    # 3. Create the Profile and link the User ID
    student = Student(
        full_name=student_in.full_name,
        semester=student_in.semester,
        user_id=new_user.id
    )
    db.add(student)
    db.commit()
    db.refresh(student)
    return student

@router.get("/students", response_model=List[StudentResponse])
def read_students(
    skip: int = 0, 
    limit: int = 100, 
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    """Retrieve all students."""
    return db.query(Student).offset(skip).limit(limit).all()

@router.get("/students/{student_id}", response_model=StudentResponse)
def read_student(
    student_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    student = db.query(Student).filter(Student.student_id == student_id).first()
    if not student:
        raise HTTPException(status_code=404, detail="Student not found")
    return student

@router.patch("/students/{student_id}", response_model=StudentResponse)
def update_student(
    student_id: int,
    update_in: StudentUpdate,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    student = db.query(Student).filter(Student.student_id == student_id).first()
    if not student:
        raise HTTPException(status_code=404, detail="Student not found")
    
    if update_in.full_name is not None:
        student.full_name = update_in.full_name
    if update_in.semester is not None:
        student.semester = update_in.semester
        
    db.commit()
    db.refresh(student)
    return student

@router.delete("/students/{student_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_student(
    student_id: int,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    student = db.query(Student).filter(Student.student_id == student_id).first()
    if not student:
        raise HTTPException(status_code=404, detail="Student not found")
    
    if student.user_id:
        user = db.query(User).filter(User.id == student.user_id).first()
        if user:
            db.delete(user)
            
    db.delete(student)
    db.commit()

# --- COLLEGE ADMINS ---
@router.post("/admins", response_model=AdminResponse, status_code=status.HTTP_201_CREATED)
def create_admin(
    admin_in: AdminCreate,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    """Add a new college admin and provision their login account."""
    # 1. Check email
    if db.query(User).filter(User.email == admin_in.email).first():
        raise HTTPException(status_code=400, detail="Email already registered.")

    # 2. Create the Auth User
    new_user = User(
        email=admin_in.email,
        hashed_password=get_password_hash(admin_in.password),
        role=UserRole.ADMIN
    )
    db.add(new_user)
    db.flush()

    # 3. Create the Profile
    admin = CollegeAdmin(
        full_name=admin_in.full_name,
        title=admin_in.title,
        department=admin_in.department,
        user_id=new_user.id
    )
    db.add(admin)
    db.commit()
    db.refresh(admin)
    return admin

@router.get("/admins", response_model=List[AdminResponse])
def read_admins(
    skip: int = 0,
    limit: int = 100,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    """Retrieve all college admins."""
    return db.query(CollegeAdmin).offset(skip).limit(limit).all()

@router.get("/admins/{admin_id}", response_model=AdminResponse)
def read_admin(
    admin_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    admin = db.query(CollegeAdmin).filter(CollegeAdmin.admin_id == admin_id).first()
    if not admin:
        raise HTTPException(status_code=404, detail="Admin not found")
    return admin

@router.patch("/admins/{admin_id}", response_model=AdminResponse)
def update_admin(
    admin_id: int,
    update_in: AdminUpdate,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    admin = db.query(CollegeAdmin).filter(CollegeAdmin.admin_id == admin_id).first()
    if not admin:
        raise HTTPException(status_code=404, detail="Admin not found")
        
    if update_in.full_name is not None:
        admin.full_name = update_in.full_name
    if update_in.title is not None:
        admin.title = update_in.title
    if update_in.department is not None:
        admin.department = update_in.department
        
    db.commit()
    db.refresh(admin)
    return admin

@router.delete("/admins/{admin_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_admin(
    admin_id: int,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    admin = db.query(CollegeAdmin).filter(CollegeAdmin.admin_id == admin_id).first()
    if not admin:
        raise HTTPException(status_code=404, detail="Admin not found")
        
    if admin.user_id:
        user = db.query(User).filter(User.id == admin.user_id).first()
        if user:
            db.delete(user)
            
    db.delete(admin)
    db.commit()


# --- PROFILE ---
@router.get("/me", response_model=Union[StudentResponse, InstructorResponse , AdminResponse])
def read_current_user_profile(
    current_user: User = Depends(get_current_user),
    db: Session = Depends(get_db)
):
    """Get the profile of the currently logged-in user."""
    # current_user is the Auth User object
    role = current_user.role
    
    if role == UserRole.STUDENT:
        # User has a student_profile relationship
        profile = current_user.student_profile
    elif role == UserRole.INSTRUCTOR:
        # User has an instructor_profile relationship
        profile = current_user.instructor_profile
    elif role == UserRole.ADMIN:
        # User has an admin_profile relationship
        profile = current_user.admin_profile
    else:
        # Should not happen if roles are strictly enforced
        raise HTTPException(status_code=403, detail="Access denied")

    # This works because Pydantic handles the inheritance/polymorphism
    return profile

@router.put("/me", response_model=Union[StudentResponse, InstructorResponse, AdminResponse])
def update_current_user_profile(
    update_in: UpdateProfileRequest,
    current_user: User = Depends(get_current_user),
    db: Session = Depends(get_db)
):
    """Update the profile of the currently logged-in user."""
    role = current_user.role
    
    if role == UserRole.STUDENT:
        profile = current_user.student_profile
        # Update fields allowed for student
        if update_in.full_name is not None:
            profile.full_name = update_in.full_name
        if update_in.semester is not None:
            profile.semester = update_in.semester
            
    elif role == UserRole.INSTRUCTOR:
        profile = current_user.instructor_profile
        if update_in.full_name is not None:
            profile.name = update_in.full_name
            
    elif role == UserRole.ADMIN:
        profile = current_user.admin_profile
        if update_in.full_name is not None:
            profile.full_name = update_in.full_name
        if update_in.title is not None:
            profile.title = update_in.title
        if update_in.department is not None:
            profile.department = update_in.department
            
    else:
        raise HTTPException(status_code=403, detail="Access denied")

    db.commit()
    db.refresh(profile)
    return profile