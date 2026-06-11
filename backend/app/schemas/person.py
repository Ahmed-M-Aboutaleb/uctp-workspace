from pydantic import BaseModel, EmailStr, ConfigDict
from typing import Optional

# --- INSTRUCTORS ---
class InstructorBase(BaseModel):
    name: str

class InstructorCreate(InstructorBase):
    email: EmailStr
    password: str

class InstructorUpdate(BaseModel):
    name: Optional[str] = None

class InstructorResponse(InstructorBase):
    instructor_id: int
    user_id: Optional[int]
    model_config = ConfigDict(from_attributes=True)

# --- STUDENTS ---
class StudentBase(BaseModel):
    full_name: str
    semester: Optional[str] = None

class StudentCreate(StudentBase):
    email: EmailStr
    password: str

class StudentUpdate(BaseModel):
    full_name: Optional[str] = None
    semester: Optional[str] = None

class StudentResponse(StudentBase):
    student_id: int
    user_id: Optional[int]
    model_config = ConfigDict(from_attributes=True)

# --- COLLEGE ADMINS ---
class AdminBase(BaseModel):
    full_name: str
    title: Optional[str] = None
    department: Optional[str] = None

class AdminCreate(AdminBase):
    email: EmailStr
    password: str

class AdminUpdate(BaseModel):
    full_name: Optional[str] = None
    title: Optional[str] = None
    department: Optional[str] = None

class AdminResponse(AdminBase):
    admin_id: int
    user_id: Optional[int]
    model_config = ConfigDict(from_attributes=True)

# --- UPDATE PROFILE ---
class UpdateProfileRequest(BaseModel):
    full_name: Optional[str] = None
    semester: Optional[str] = None
    title: Optional[str] = None
    department: Optional[str] = None