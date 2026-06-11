import json
import os
from sqlalchemy.orm import Session
from app.models.assignment import TimetableAssignment

def save_solver_results(db: Session, result_folder: str, course_ids: list, room_ids: list):
    """
    Parses run_1_timetable.json and saves it to the database.
    """
    summary_path = os.path.join(result_folder, "run_1_timetable.json")
    
    with open(summary_path, 'r') as f:
        data = json.load(f)

    # Clear old assignments before saving new ones (optional, depends on your preference)
    db.query(TimetableAssignment).delete()

    for item in data['schedule']:
        # Map C++ index back to DB Primary Key
        db_course_id = course_ids[item['event_id']]
        # room_id is -1 if the solver failed to place it (HCV)
        db_room_id = room_ids[item['room_id']] if item['room_id'] != -1 else None
        
        if db_room_id:
            assignment = TimetableAssignment(
                course_id=db_course_id,
                room_id=db_room_id,
                timeslot_id=item['timeslot_id']
            )
            db.add(assignment)
    
    db.commit()