from pydantic_settings import BaseSettings, SettingsConfigDict
from typing import List

class Settings(BaseSettings):
    # App Config
    PROJECT_NAME: str = "UCTP Solver API"
    VERSION: str = "1.0.0"
    API_V1_STR: str = "/api"
    
    # CORS Config
    BACKEND_CORS_ORIGINS: List[str] = ["http://localhost:3000", "http://127.0.0.1:3000", "http://0.0.0.0:3000", "http://192.168.1.18:3000"]
    
    # PostgreSQL Config
    POSTGRES_USER: str
    POSTGRES_PASSWORD: str
    POSTGRES_DB: str
    DATABASE_URL: str
    
    # Redis Config
    REDIS_URL: str
    
    # JWT Config
    SECRET_KEY: str
    ALGORITHM: str = "HS256"
    ACCESS_TOKEN_EXPIRE_MINUTES: int = 60 * 24 * 7 # 7 days
    
    # Initial Admin Setup
    FIRST_SUPERUSER_EMAIL: str
    FIRST_SUPERUSER_PASSWORD: str
    
    # Pydantic v2 configuration
    model_config = SettingsConfigDict(
        env_file=".env", 
        env_file_encoding="utf-8", 
        extra="ignore" # Ignores extra variables in the .env file not defined here
    )

settings = Settings()