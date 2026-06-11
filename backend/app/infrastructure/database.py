from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from app.infrastructure.settings import settings

# Use the validated DATABASE_URL from Pydantic
engine = create_engine(settings.DATABASE_URL, echo=False)

SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

def get_db():
    """
    Dependency generator to provide a database session per request.
    Closes the session automatically after the request finishes.
    """
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()