from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy import String, ForeignKey
from typing import List, Optional
from app.models.base import Base
from app.models.junctions import student_course_table, instructor_course_table

class Student(Base):
    __tablename__ = "student"

    student_id: Mapped[int] = mapped_column(primary_key=True, autoincrement=True)
    full_name: Mapped[str] = mapped_column(String(255), nullable=False)
    semester: Mapped[str] = mapped_column(String(50), nullable=True)

    # NEW: Link to the Auth User
    user_id: Mapped[Optional[int]] = mapped_column(ForeignKey("user.id", ondelete="SET NULL"), unique=True, nullable=True)
    user: Mapped["User"] = relationship(back_populates="student_profile")

    courses: Mapped[List["Course"]] = relationship(secondary=student_course_table, back_populates="students")

class Instructor(Base):
    __tablename__ = "instructor"

    instructor_id: Mapped[int] = mapped_column(primary_key=True, autoincrement=True)
    name: Mapped[str] = mapped_column(String(255), nullable=False)

    # NEW: Link to the Auth User
    user_id: Mapped[Optional[int]] = mapped_column(ForeignKey("user.id", ondelete="SET NULL"), unique=True, nullable=True)
    user: Mapped["User"] = relationship(back_populates="instructor_profile")

    courses: Mapped[List["Course"]] = relationship(secondary=instructor_course_table, back_populates="instructors")
    
    
class CollegeAdmin(Base):
    __tablename__ = "college_admin"

    admin_id: Mapped[int] = mapped_column(primary_key=True, autoincrement=True)
    full_name: Mapped[str] = mapped_column(String(255), nullable=False)
    title: Mapped[str] = mapped_column(String(100), nullable=True) # e.g., "Dean of Students", "Department Head"
    department: Mapped[str] = mapped_column(String(100), nullable=True) 

    # Link to the Auth User
    user_id: Mapped[Optional[int]] = mapped_column(ForeignKey("user.id", ondelete="SET NULL"), unique=True, nullable=True)
    user: Mapped["User"] = relationship(back_populates="admin_profile")