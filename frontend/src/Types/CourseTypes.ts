export interface CourseInstructor {
  instructor_id: number;
  name: string;
}

export interface CourseStudent {
  student_id: number;
  full_name: string;
  semester?: string;
}

export interface CourseFeature {
  feature_id: number;
  feature_name: string;
}

export interface Course {
  course_id: number;
  course_name: string;
  instructors: CourseInstructor[];
  students: CourseStudent[];
  features: CourseFeature[];
  is_enrollment_open: boolean;
  course_availability: number[];
  precedence_rules: number[];
}

export type CourseResponse = Course[];

// Used for POST /courses
export interface CreateCourseData {
  course_name: string;
  student_ids: number[];
  instructor_ids: number[];
  feature_ids: number[];
  is_enrollment_open?: boolean;
  course_availability?: number[];
  precedence_rules?: number[];
}

// Used for PUT/PATCH /courses/:id  — no student_ids
export interface UpdateCourseData {
  course_name: string;
  instructor_ids: number[];
  feature_ids: number[];
  is_enrollment_open?: boolean;
  course_availability?: number[];
  precedence_rules?: number[];
}