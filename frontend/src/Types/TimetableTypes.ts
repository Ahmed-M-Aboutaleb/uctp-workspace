export type TimetableEntry = {
    message: string;
    job_id: string;
    status: string;
}



export interface Feature {
  feature_id: number;
  feature_name: string;
}

export interface Instructor {
  instructor_id: number;
  user_id: number;
  name: string;
}

export interface Student {
  student_id: number;
  user_id: number;
  full_name: string;
  semester: "Fall" | "Spring" | "Summer";
}

export interface Course {
  course_id: number;
  course_name: string;
  instructors: Instructor[];
  students: Student[];
  features: Feature[]; 
}

export interface Room {
  room_id: number;
  capacity: number;
  features: Feature[];
}

export interface TimeSlotResponse {
  id: number;
  timeslot_id: number;
  course: Course;
  room: Room;
}

export type TimetableResponse = TimeSlotResponse[];



