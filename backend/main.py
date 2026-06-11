from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.infrastructure.settings import settings
from app.infrastructure.logger import get_logger
from app.api.v1.endpoints import auth, facilities, people, courses, enrollment, solver, timetable

logger = get_logger(__name__)

app = FastAPI(
    title=settings.PROJECT_NAME,
    version=settings.VERSION,
    description="Backend orchestration for the C++ Algorithm timetabling engine."
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.BACKEND_CORS_ORIGINS, 
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.on_event("startup")
async def startup_event():
    logger.info("🚀 UCTP FastAPI Server is starting up...")

app.include_router(auth.router, prefix=f"{settings.API_V1_STR}/auth", tags=["auth"])
app.include_router(facilities.router, prefix=f"{settings.API_V1_STR}/facilities", tags=["facilities"])
app.include_router(people.router, prefix=f"{settings.API_V1_STR}/people", tags=["people"])
app.include_router(courses.router, prefix=f"{settings.API_V1_STR}/courses", tags=["courses"])
app.include_router(enrollment.router, prefix=f"{settings.API_V1_STR}/enrollment", tags=["enrollment"]) 
app.include_router(solver.router, prefix=f"{settings.API_V1_STR}/solver", tags=["solver"]) # Add this line!
app.include_router(timetable.router, prefix=f"{settings.API_V1_STR}/timetable", tags=["timetable"]) # Add this line

@app.get("/")
def read_root():
    return {"status": "online", "message": f"{settings.PROJECT_NAME} is running"}

@app.get(f"{settings.API_V1_STR}/health")
def health_check():
    return {"status": "healthy"}