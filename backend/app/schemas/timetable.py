from pydantic import BaseModel, ConfigDict
from app.schemas.course import CourseResponse
from app.schemas.facility import RoomResponse
from typing import Optional

class TimetableAssignmentCreate(BaseModel):
    timeslot_id: int
    course_id: int
    room_id: int

class TimetableAssignmentUpdate(BaseModel):
    timeslot_id: Optional[int] = None
    course_id: Optional[int] = None
    room_id: Optional[int] = None

class TimetableAssignmentResponse(BaseModel):
    id: int
    timeslot_id: int
    course: CourseResponse
    room: RoomResponse

    model_config = ConfigDict(from_attributes=True)