from sqlalchemy import Table, Column, ForeignKey
from app.models.base import Base

student_course_table = Table(
    "student_course",
    Base.metadata,
    Column("student_id", ForeignKey("student.student_id", ondelete="CASCADE"), primary_key=True),
    Column("course_id", ForeignKey("course.course_id", ondelete="CASCADE"), primary_key=True),
)

instructor_course_table = Table(
    "instructor_course",
    Base.metadata,
    Column("instructor_id", ForeignKey("instructor.instructor_id", ondelete="CASCADE"), primary_key=True),
    Column("course_id", ForeignKey("course.course_id", ondelete="CASCADE"), primary_key=True),
)

room_feature_table = Table(
    "room_feature",
    Base.metadata,
    Column("room_id", ForeignKey("room.room_id", ondelete="CASCADE"), primary_key=True),
    Column("feature_id", ForeignKey("feature.feature_id", ondelete="CASCADE"), primary_key=True),
)

course_feature_table = Table(
    "course_feature",
    Base.metadata,
    Column("course_id", ForeignKey("course.course_id", ondelete="CASCADE"), primary_key=True),
    Column("feature_id", ForeignKey("feature.feature_id", ondelete="CASCADE"), primary_key=True),
)