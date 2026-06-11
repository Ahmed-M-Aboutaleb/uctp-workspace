import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
import logging
import random
from sqlalchemy.orm import Session
from app.infrastructure.database import SessionLocal
from app.core.security import get_password_hash
from app.models.user import User, UserRole
from app.models.facilities import Room, Feature
from app.models.people import Instructor, Student
from app.models.course import Course
from app.models.assignment import TimetableAssignment

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def clean_database(db: Session):
    logger.info("Sweeping old data...")
    db.query(TimetableAssignment).delete()
    db.query(Course).delete()
    db.query(Student).delete()
    db.query(Instructor).delete()
    db.query(Room).delete()
    db.query(Feature).delete()
    # Delete all users except the Admin
    db.query(User).filter(User.role != UserRole.ADMIN).delete()
    db.commit()

def seed_database(db: Session):
    logger.info("Initializing Stress Test Dataset...")
    
    # Cache one password hash so the script runs in 1 second instead of 2 minutes
    fast_hash = get_password_hash("pass123")

    # 1. Features
    feature_names = ["Projector", "Whiteboard", "Linux Lab", "Mac Lab", "Chemistry Eq", "Physics Eq", "Smart Board", "Sound System", "Drafting Tables", "3D Printers"]
    features = [Feature(feature_name=name) for name in feature_names]
    db.add_all(features)
    db.flush()

    # 2. Rooms
    rooms = []
    for i in range(15):
        rooms.append(Room(capacity=500, features=features)) # Pass the entire features list!
    db.add_all(rooms)
    db.flush()

    # 3. Instructors
    instructors = []
    for i in range(1, 21):
        user = User(email=f"instructor{i}@uni.edu", hashed_password=fast_hash, role=UserRole.INSTRUCTOR)
        db.add(user)
        db.flush()
        instructor = Instructor(name=f"Dr. Professor {i}", user_id=user.id)
        db.add(instructor)
        instructors.append(instructor)
    db.flush()

    # 4. Students
    students = []
    for i in range(1, 201):
        user = User(email=f"student{i}@uni.edu", hashed_password=fast_hash, role=UserRole.STUDENT)
        db.add(user)
        db.flush()
        student = Student(full_name=f"Student Number {i}", semester=random.choice(["Fall", "Spring", "Summer"]), user_id=user.id)
        db.add(student)
        students.append(student)
    db.flush()

    # 5. Courses
    courses = []
    course_prefixes = ["CS", "MATH", "PHY", "ENG", "BIO", "CHEM", "ART", "HIST", "ECO", "BUS"]
    for i in range(1, 61):
        # Pick 1 or 2 instructors
        course_instructors = random.sample(instructors, random.randint(1, 2))
        # Pick 0 to 2 required features
        course_features = random.sample(features, random.randint(0, 2))
        
        course = Course(
            course_name=f"{random.choice(course_prefixes)} {random.randint(100, 499)}",
            is_enrollment_open=False, # Closed for solver
            course_availability=[1] * 45,
            precedence_rules=[0] * 60,
            instructors=course_instructors,
            features=course_features,
            students=[] # We will append students next
        )
        courses.append(course)
    
    # 6. Create the Conflict Web (Enrollment)
    # Give every student 4 to 6 random courses
    for student in students:
        enrolled_courses = random.sample(courses, random.randint(4, 6))
        for c in enrolled_courses:
            c.students.append(student)

    db.add_all(courses)
    db.commit()

    logger.info(f"✅ Generated {len(courses)} Courses, {len(rooms)} Rooms, {len(students)} Students, and {len(instructors)} Instructors.")

if __name__ == "__main__":
    db = SessionLocal()
    try:
        clean_database(db)
        seed_database(db)
    finally:
        db.close()