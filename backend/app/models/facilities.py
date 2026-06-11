from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy import String, Integer
from typing import List
from app.models.base import Base
from app.models.junctions import room_feature_table, course_feature_table

class Feature(Base):
    __tablename__ = "feature"

    feature_id: Mapped[int] = mapped_column(primary_key=True, autoincrement=True)
    feature_name: Mapped[str] = mapped_column(String(100), nullable=False)

    # Relationships
    rooms: Mapped[List["Room"]] = relationship(secondary=room_feature_table, back_populates="features")
    courses: Mapped[List["Course"]] = relationship(secondary=course_feature_table, back_populates="features")

class Room(Base):
    __tablename__ = "room"

    room_id: Mapped[int] = mapped_column(primary_key=True, autoincrement=True)
    capacity: Mapped[int] = mapped_column(Integer, nullable=False)

    # Relationships
    features: Mapped[List["Feature"]] = relationship(secondary=room_feature_table, back_populates="rooms")