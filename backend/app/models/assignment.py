from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy import ForeignKey, Integer
from app.models.base import Base

class TimetableAssignment(Base):
    __tablename__ = "timetable_assignment"

    id: Mapped[int] = mapped_column(primary_key=True, autoincrement=True)
    
    # Links to the actual entities
    course_id: Mapped[int] = mapped_column(ForeignKey("course.course_id"), nullable=False)
    room_id: Mapped[int] = mapped_column(ForeignKey("room.room_id"), nullable=False)
    
    # The result from the solver (0-44)
    timeslot_id: Mapped[int] = mapped_column(Integer, nullable=False)

    # Relationships for easy querying on the frontend
    course: Mapped["Course"] = relationship()
    room: Mapped["Room"] = relationship()