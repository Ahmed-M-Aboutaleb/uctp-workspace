from pydantic import BaseModel, EmailStr, ConfigDict
from app.models.user import UserRole

# For creating a new user (Registration)
class UserCreate(BaseModel):
    email: EmailStr
    password: str
    role: UserRole = UserRole.STUDENT

# What the API returns (Never send the hashed_password back!)
class UserResponse(BaseModel):
    id: int
    email: EmailStr
    role: UserRole
    is_active: bool

    model_config = ConfigDict(from_attributes=True)


# For login payloads (JWT generation)
class Token(BaseModel):
    access_token: str
    token_type: str = "bearer"

class TokenData(BaseModel):
    email: str | None = None
    role: UserRole | None = None

class LoginResponse(BaseModel):
    access_token: str
    token_type: str = "bearer"
    user: UserResponse