from pydantic import BaseModel, ConfigDict
from typing import List, Optional
from app.schemas.person import InstructorResponse, StudentResponse
from app.schemas.facility import FeatureResponse

class CourseBase(BaseModel):
    course_name: str

class CourseCreate(CourseBase):
    # Admin only assigns instructors and features at creation
    student_ids: Optional[List[int]] = []
    instructor_ids: Optional[List[int]] = []
    feature_ids: Optional[List[int]] = []
    is_enrollment_open: Optional[bool] = False
    course_availability: Optional[List[int]] = None
    precedence_rules: Optional[List[int]] = None

class CourseUpdate(BaseModel):
    course_name: Optional[str] = None
    is_enrollment_open: Optional[bool] = None
    course_availability: Optional[List[int]] = None
    precedence_rules: Optional[List[int]] = None
    instructor_ids: Optional[List[int]] = None
    feature_ids: Optional[List[int]] = None

class CourseResponse(CourseBase):
    course_id: int
    instructors: List[InstructorResponse] = []
    students: List[StudentResponse] = [] # Students will populate later during enrollment
    features: List[FeatureResponse] = []
    is_enrollment_open: bool = False
    course_availability: List[int] = [1] * 45
    precedence_rules: List[int] = []
    
    model_config = ConfigDict(from_attributes=True)