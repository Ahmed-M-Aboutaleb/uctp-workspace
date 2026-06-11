from app.models.base import Base
from app.models.user import User, UserRole
from app.models.junctions import (
    student_course_table, 
    instructor_course_table, 
    room_feature_table, 
    course_feature_table
)
from app.models.people import Student, Instructor, CollegeAdmin
from app.models.facilities import Room, Feature
from app.models.course import Course
from app.models.schedule import Timeslot, ScheduleRun, ScheduleAssignment
from app.models.assignment import TimetableAssignment