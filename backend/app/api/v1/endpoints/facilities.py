from typing import List
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session

from app.infrastructure.database import get_db
from app.api.deps import get_current_admin, get_current_user
from app.models.user import User
from app.models.facilities import Room, Feature
from app.schemas.facility import RoomCreate, RoomResponse, RoomUpdate, FeatureCreate, FeatureResponse, FeatureUpdate

router = APIRouter()

# --- FEATURES ENDPOINTS ---

@router.post("/features", response_model=FeatureResponse, status_code=status.HTTP_201_CREATED)
def create_feature(
    feature_in: FeatureCreate, 
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin) # LOCKED TO ADMINS
):
    """Create a new facility feature (e.g., 'Projector', 'Chemistry Lab')."""
    feature = Feature(feature_name=feature_in.feature_name)
    db.add(feature)
    db.commit()
    db.refresh(feature)
    return feature

@router.get("/features", response_model=List[FeatureResponse])
def read_features(
    skip: int = 0, 
    limit: int = 100, 
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user) # Any logged-in user can view
):
    """Retrieve all available features."""
    features = db.query(Feature).offset(skip).limit(limit).all()
    return features

@router.get("/features/{feature_id}", response_model=FeatureResponse)
def read_feature(
    feature_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    feature = db.query(Feature).filter(Feature.feature_id == feature_id).first()
    if not feature:
        raise HTTPException(status_code=404, detail="Feature not found")
    return feature

@router.patch("/features/{feature_id}", response_model=FeatureResponse)
def update_feature(
    feature_id: int,
    update_in: FeatureUpdate,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    feature = db.query(Feature).filter(Feature.feature_id == feature_id).first()
    if not feature:
        raise HTTPException(status_code=404, detail="Feature not found")
        
    if update_in.feature_name is not None:
        feature.feature_name = update_in.feature_name
        
    db.commit()
    db.refresh(feature)
    return feature

@router.delete("/features/{feature_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_feature(
    feature_id: int,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    feature = db.query(Feature).filter(Feature.feature_id == feature_id).first()
    if not feature:
        raise HTTPException(status_code=404, detail="Feature not found")
        
    db.delete(feature)
    db.commit()
    return {
        "message": "Feature deleted successfully"
    }

# --- ROOMS ENDPOINTS ---

@router.post("/rooms", response_model=RoomResponse, status_code=status.HTTP_201_CREATED)
def create_room(
    room_in: RoomCreate, 
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin) # LOCKED TO ADMINS
):
    """Create a new room and assign its capabilities."""
    room = Room(capacity=room_in.capacity)
    
    # If the admin provided feature IDs, link them to the room
    if room_in.feature_ids:
        features = db.query(Feature).filter(Feature.feature_id.in_(room_in.feature_ids)).all()
        if len(features) != len(room_in.feature_ids):
            raise HTTPException(status_code=404, detail="One or more features not found")
        room.features = features

    db.add(room)
    db.commit()
    db.refresh(room)
    return room

@router.get("/rooms", response_model=List[RoomResponse])
def read_rooms(
    skip: int = 0, 
    limit: int = 100, 
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user) # Any logged-in user can view
):
    """Retrieve all rooms and their associated features."""
    rooms = db.query(Room).offset(skip).limit(limit).all()
    return rooms

@router.get("/rooms/{room_id}", response_model=RoomResponse)
def read_room(
    room_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user)
):
    room = db.query(Room).filter(Room.room_id == room_id).first()
    if not room:
        raise HTTPException(status_code=404, detail="Room not found")
    return room

@router.patch("/rooms/{room_id}", response_model=RoomResponse)
def update_room(
    room_id: int,
    update_in: RoomUpdate,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    room = db.query(Room).filter(Room.room_id == room_id).first()
    if not room:
        raise HTTPException(status_code=404, detail="Room not found")
        
    if update_in.capacity is not None:
        room.capacity = update_in.capacity
        
    if update_in.feature_ids is not None:
        features = db.query(Feature).filter(Feature.feature_id.in_(update_in.feature_ids)).all()
        if len(features) != len(update_in.feature_ids):
            raise HTTPException(status_code=404, detail="One or more features not found")
        room.features = features
        
    db.commit()
    db.refresh(room)
    return room

@router.delete("/rooms/{room_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_room(
    room_id: int,
    db: Session = Depends(get_db),
    current_admin: User = Depends(get_current_admin)
):
    room = db.query(Room).filter(Room.room_id == room_id).first()
    if not room:
        raise HTTPException(status_code=404, detail="Room not found")
        
    db.delete(room)
    db.commit()
    return {
        "message": "Room deleted successfully"
    }