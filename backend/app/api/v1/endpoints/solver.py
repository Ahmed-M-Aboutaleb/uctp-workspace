from fastapi import APIRouter, Depends, status
from sqlalchemy.orm import Session
import os
from redis import Redis
from rq import Queue

from app.infrastructure.database import get_db
from app.infrastructure.settings import settings
from app.api.deps import get_current_admin
from app.models.user import User
from app.solver.mapper import TimetableMapper
from app.worker.tasks import generate_timetable_task

router = APIRouter()

# Connect to the Redis container using the URL from your .env
redis_conn = Redis.from_url(settings.REDIS_URL)
task_queue = Queue("high", connection=redis_conn)

@router.post("/generate", status_code=status.HTTP_202_ACCEPTED)
def trigger_solver_generation(
    current_admin: User = Depends(get_current_admin)
):
    """
    Sends the generation task to the Redis Queue to be processed by the worker.
    """
    # Enqueue the background task
    job = task_queue.enqueue(generate_timetable_task)
    
    return {
        "message": "Timetable generation started in the background.",
        "job_id": job.id,
        "status": "queued"
    }

@router.post("/export", status_code=status.HTTP_200_OK)
def export_database_to_tim(
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin) # Only Dean can run the solver
):
    """Generates the .dat file from the current database state."""
    
    mapper = TimetableMapper(db=db)
    
    # Generate a unique filename based on the current run, or overwrite a standard one
    file_path = mapper.generate_dat_file(filename="current_semester.dat")
    
    # Just to verify the file size for the response
    file_size = os.path.getsize(file_path)

    return {
        "message": "Data successfully mapped for the C++ solver.",
        "file_path": file_path,
        "bytes": file_size
    }