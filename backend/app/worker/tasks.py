from app.infrastructure.database import SessionLocal
from app.solver.mapper import TimetableMapper
from app.solver.engine import SolverEngine
from app.infrastructure.logger import get_logger
from app.solver.parser import save_solver_results

logger = get_logger(__name__)

def generate_timetable_task():
    logger.info("--- Background Task: Timetable Generation Started ---")
    db = SessionLocal()
    
    try:
        # UPDATED: Generate .dat instead of .tim
        mapper = TimetableMapper(db=db)
        input_filepath, c_ids, r_ids = mapper.generate_dat_file(filename="latest_run.dat")
        
        engine = SolverEngine()
        output_filepath = engine.execute(input_filepath=input_filepath)
        save_solver_results(db, output_filepath, c_ids, r_ids)
        logger.info(f"Task Complete! Results waiting at: {output_filepath}")
        
    except Exception as e:
        logger.error(f"Timetable generation failed: {str(e)}")
    finally:
        db.close()