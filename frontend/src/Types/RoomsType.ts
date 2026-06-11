export interface Feature {
  feature_id: number;
  feature_name: string;
}

export interface Room {
  room_id: number;
  capacity: number;
  features: Feature[];
}

export interface RoomFormData {
  capacity: number
  feature_ids: number[] 
}
// If you are typing the API response itself:
export type RoomsResponse = Room[];