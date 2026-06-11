from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy import ForeignKey, String, Integer, DateTime
from datetime import datetime, timezone
from typing import List
from app.models.base import Base

class Timeslot(Base):
    __tablename__ = "timeslot"
    
    # We don't autoincrement here because the C++ solver dictates timeslots (e.g., 0 to 44)
    timeslot_id: Mapped[int] = mapped_column(primary_key=True)
    # You can add a description column later (e.g., "Monday 9:00 AM")

class ScheduleRun(Base):
    __tablename__ = "schedule_run"

    id: Mapped[int] = mapped_column(primary_key=True, autoincrement=True)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=lambda: datetime.now(timezone.utc))
    seed: Mapped[int] = mapped_column(Integer)
    
    # Split the C++ JSON fitness object for easier querying in the DB
    fitness_score: Mapped[int] = mapped_column(Integer)
    hcv: Mapped[int] = mapped_column(Integer)
    scv: Mapped[int] = mapped_column(Integer)
    
    metadata_info: Mapped[str] = mapped_column(String, nullable=True)

    # Relationships
    assignments: Mapped[List["ScheduleAssignment"]] = relationship(back_populates="schedule_run", cascade="all, delete-orphan")

class ScheduleAssignment(Base):
    __tablename__ = "schedule_assignment"

    assignment_id: Mapped[int] = mapped_column(primary_key=True, autoincrement=True)
    schedule_id: Mapped[int] = mapped_column(ForeignKey("schedule_run.id", ondelete="CASCADE"))
    course_id: Mapped[int] = mapped_column(ForeignKey("course.course_id", ondelete="CASCADE"))
    room_id: Mapped[int] = mapped_column(ForeignKey("room.room_id", ondelete="CASCADE"))
    timeslot_id: Mapped[int] = mapped_column(ForeignKey("timeslot.timeslot_id", ondelete="CASCADE"))

    # Relationships
    schedule_run: Mapped["ScheduleRun"] = relationship(back_populates="assignments")