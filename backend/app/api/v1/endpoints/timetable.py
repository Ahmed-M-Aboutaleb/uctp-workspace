from typing import List
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session, joinedload

from app.infrastructure.database import get_db
from app.api.deps import get_current_user, get_current_admin
from app.models.user import User
from app.models.assignment import TimetableAssignment
from app.schemas.timetable import TimetableAssignmentResponse, TimetableAssignmentCreate, TimetableAssignmentUpdate

router = APIRouter()

@router.get("/", response_model=List[TimetableAssignmentResponse])
def get_current_timetable(
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user) # Anyone logged in can view the schedule
):
    """
    Fetches the latest generated timetable with full course and room details.
    """
    assignments = db.query(TimetableAssignment)\
        .options(
            joinedload(TimetableAssignment.course),
            joinedload(TimetableAssignment.room)
        ).all()
    return assignments

@router.post("/", response_model=TimetableAssignmentResponse, status_code=status.HTTP_201_CREATED)
def create_assignment(
    assignment_in: TimetableAssignmentCreate,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    assignment = TimetableAssignment(
        timeslot_id=assignment_in.timeslot_id,
        course_id=assignment_in.course_id,
        room_id=assignment_in.room_id
    )
    db.add(assignment)
    db.commit()
    db.refresh(assignment)
    # Reload with relationships
    return db.query(TimetableAssignment).options(joinedload(TimetableAssignment.course), joinedload(TimetableAssignment.room)).filter(TimetableAssignment.id == assignment.id).first()

@router.get("/{assignment_id}", response_model=TimetableAssignmentResponse)
def read_assignment(
    assignment_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    assignment = db.query(TimetableAssignment)\
        .options(joinedload(TimetableAssignment.course), joinedload(TimetableAssignment.room))\
        .filter(TimetableAssignment.id == assignment_id).first()
    if not assignment:
        raise HTTPException(status_code=404, detail="Assignment not found")
    return assignment

@router.patch("/{assignment_id}", response_model=TimetableAssignmentResponse)
def update_assignment(
    assignment_id: int,
    update_in: TimetableAssignmentUpdate,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    assignment = db.query(TimetableAssignment).filter(TimetableAssignment.id == assignment_id).first()
    if not assignment:
        raise HTTPException(status_code=404, detail="Assignment not found")
        
    if update_in.timeslot_id is not None:
        assignment.timeslot_id = update_in.timeslot_id
    if update_in.course_id is not None:
        assignment.course_id = update_in.course_id
    if update_in.room_id is not None:
        assignment.room_id = update_in.room_id
        
    db.commit()
    db.refresh(assignment)
    return db.query(TimetableAssignment).options(joinedload(TimetableAssignment.course), joinedload(TimetableAssignment.room)).filter(TimetableAssignment.id == assignment.id).first()

@router.delete("/{assignment_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_assignment(
    assignment_id: int,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    assignment = db.query(TimetableAssignment).filter(TimetableAssignment.id == assignment_id).first()
    if not assignment:
        raise HTTPException(status_code=404, detail="Assignment not found")
        
    db.delete(assignment)
    db.commit()