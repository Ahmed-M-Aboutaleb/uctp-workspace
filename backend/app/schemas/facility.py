from pydantic import BaseModel, ConfigDict
from typing import List, Optional

# --- FEATURES ---
class FeatureBase(BaseModel):
    feature_name: str

class FeatureCreate(FeatureBase):
    pass

class FeatureUpdate(BaseModel):
    feature_name: Optional[str] = None

class FeatureResponse(FeatureBase):
    feature_id: int
    model_config = ConfigDict(from_attributes=True)

# --- ROOMS ---
class RoomBase(BaseModel):
    capacity: int

class RoomCreate(RoomBase):
    feature_ids: Optional[List[int]] = [] # Allow attaching features on creation

class RoomUpdate(BaseModel):
    capacity: Optional[int] = None
    feature_ids: Optional[List[int]] = None

class RoomResponse(RoomBase):
    room_id: int
    features: List[FeatureResponse] = []
    model_config = ConfigDict(from_attributes=True)