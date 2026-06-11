import os
import logging
from pathlib import Path
from sqlalchemy.orm import Session
from app.models.course import Course
from app.models.facilities import Room, Feature
from app.models.people import Student
from app.infrastructure.logger import get_logger

logger = get_logger(__name__)
BASE_DIR = Path(__file__).resolve().parent

class TimetableMapper:
    def __init__(self, db: Session, output_dir: str = str(BASE_DIR / "workspace" / "inputs")):
        self.db = db
        self.output_dir = output_dir
        os.makedirs(self.output_dir, exist_ok=True)

    def generate_dat_file(self, filename: str = "instance.dat") -> str:
        """
        Extracts database state and generates a strict integer matrix .dat file.
        """
        file_path = os.path.join(self.output_dir, filename)
        
        # 1. Fetch the exact state of the university
        active_courses = self.db.query(Course).filter(Course.students.any()).all()
        rooms = self.db.query(Room).all()
        features = self.db.query(Feature).all()
        students = self.db.query(Student).all()

        num_events = len(active_courses)
        num_rooms = len(rooms)
        num_features = len(features)
        num_students = len(students)

        logger.info(f"Mapping Matrix -> Events: {num_events}, Rooms: {num_rooms}, Features: {num_features}, Students: {num_students}")
        ordered_course_ids = [c.course_id for c in active_courses]
        ordered_room_ids = [r.room_id for r in rooms]
        with open(file_path, 'w') as f:
            # BLOCK 1: Header Row
            f.write(f"{num_events} {num_rooms} {num_features} {num_students}\n")

            # BLOCK 2: Room Capacities
            for room in rooms:
                f.write(f"{room.capacity}\n")

            # BLOCK 3: Student-Event Matrix (num_students x num_events)
            for student in students:
                # Precompute for fast lookup
                student_course_ids = {c.course_id for c in student.courses}
                for course in active_courses:
                    if course.course_id in student_course_ids:
                        f.write("1\n")
                    else:
                        f.write("0\n")

            # BLOCK 4: Room-Feature Matrix (num_rooms x num_features)
            for room in rooms:
                room_feature_ids = {feat.feature_id for feat in room.features}
                for feature in features:
                    if feature.feature_id in room_feature_ids:
                        f.write("1\n")
                    else:
                        f.write("0\n")

            # BLOCK 5: Event-Feature Matrix (num_events x num_features)
            for course in active_courses:
                course_feature_ids = {feat.feature_id for feat in course.features}
                for feature in features:
                    if feature.feature_id in course_feature_ids:
                        f.write("1\n")
                    else:
                        f.write("0\n")

            # BLOCK 6: Course Availability Matrix (num_events x 45)
            for course in active_courses:
                for t in range(45):
                    if course.course_availability and len(course.course_availability) > t:
                        f.write(f"{course.course_availability[t]}\n")
                    else:
                        f.write("1\n")

            # BLOCK 7: Precedence Rules Matrix (num_events x num_events)
            for course in active_courses:
                for other_course in active_courses:
                    idx = active_courses.index(other_course)
                    if course.precedence_rules and len(course.precedence_rules) > idx:
                        f.write(f"{course.precedence_rules[idx]}\n")
                    else:
                        f.write("0\n")

        logger.info(f"✅ Successfully generated strictly formatted .dat file at {file_path}")
        return file_path, ordered_course_ids, ordered_room_ids