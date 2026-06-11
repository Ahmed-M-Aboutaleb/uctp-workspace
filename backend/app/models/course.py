from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy import String, Boolean, JSON
from typing import List
from app.models.base import Base
from app.models.junctions import student_course_table, instructor_course_table, course_feature_table

class Course(Base):
    __tablename__ = "course"

    course_id: Mapped[int] = mapped_column(primary_key=True, autoincrement=True)
    course_name: Mapped[str] = mapped_column(String(255), nullable=False)

    is_enrollment_open: Mapped[bool] = mapped_column(Boolean, default=False, nullable=False)
    course_availability: Mapped[list] = mapped_column(JSON, default=lambda: [1] * 45, nullable=False)
    precedence_rules: Mapped[list] = mapped_column(JSON, default=list, nullable=False)
    # Relationships mapping back to the junction tables
    students: Mapped[List["Student"]] = relationship(secondary=student_course_table, back_populates="courses")
    instructors: Mapped[List["Instructor"]] = relationship(secondary=instructor_course_table, back_populates="courses")
    features: Mapped[List["Feature"]] = relationship(secondary=course_feature_table, back_populates="courses")