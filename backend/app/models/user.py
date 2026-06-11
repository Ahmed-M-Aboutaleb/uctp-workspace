import enum
from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy import String, Enum, Boolean
from app.models.base import Base

class UserRole(str, enum.Enum):
    ADMIN = "admin"
    INSTRUCTOR = "instructor"
    STUDENT = "student"

class User(Base):
    __tablename__ = "user"

    id: Mapped[int] = mapped_column(primary_key=True, autoincrement=True)
    email: Mapped[str] = mapped_column(String(255), unique=True, index=True, nullable=False)
    hashed_password: Mapped[str] = mapped_column(String(255), nullable=False)
    role: Mapped[UserRole] = mapped_column(Enum(UserRole), default=UserRole.STUDENT, nullable=False)
    is_active: Mapped[bool] = mapped_column(Boolean, default=True)

    # 1-to-1 Relationships linking the login account to the domain profile
    student_profile: Mapped["Student"] = relationship(back_populates="user", uselist=False)
    instructor_profile: Mapped["Instructor"] = relationship(back_populates="user", uselist=False)
    admin_profile: Mapped["CollegeAdmin"] = relationship(back_populates="user", uselist=False)