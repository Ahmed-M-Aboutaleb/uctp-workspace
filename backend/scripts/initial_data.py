import sys
import os
# Add the parent directory (backend) to the Python path so we can import modules from app/
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from sqlalchemy.orm import Session
from app.infrastructure.database import SessionLocal
from app.infrastructure.settings import settings
from app.infrastructure.logger import get_logger
from app.core.security import get_password_hash
from app.models.user import User, UserRole
from app.models.people import CollegeAdmin

# Setup logging to see what the script is doing
logger = get_logger(__name__)

def init_db(db: Session) -> None:
    # 1. Check if the user already exists to prevent duplicate errors
    user = db.query(User).filter(User.email == settings.FIRST_SUPERUSER_EMAIL).first()
    
    if not user:
        logger.info(f"Creating initial superuser: {settings.FIRST_SUPERUSER_EMAIL}")
        
        # 2. Create the User account with the hashed password
        user = User(
            email=settings.FIRST_SUPERUSER_EMAIL,
            hashed_password=get_password_hash(settings.FIRST_SUPERUSER_PASSWORD),
            role=UserRole.ADMIN,
            is_active=True,
        )
        db.add(user)
        db.commit()
        db.refresh(user) # Refresh to get the auto-generated user.id

        # 3. Create the CollegeAdmin profile linked to this user
        logger.info("Creating CollegeAdmin profile...")
        admin_profile = CollegeAdmin(
            full_name="System Administrator",
            title="Dean of Scheduling",
            department="University Administration",
            user_id=user.id
        )
        db.add(admin_profile)
        db.commit()
        
        logger.info("✅ Initial superuser and admin profile created successfully.")
    else:
        logger.info("⚠️ Initial superuser already exists. Skipping creation.")

def main() -> None:
    logger.info("--- Starting Database Initialization ---")
    db = SessionLocal()
    try:
        init_db(db)
    finally:
        db.close()
    logger.info("--- Database Initialization Complete ---")

if __name__ == "__main__":
    main()