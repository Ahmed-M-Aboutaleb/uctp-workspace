type Semester = 'Fall' | 'Spring' | 'Summer';

type CourseFeature = {
  feature_name: string;
  feature_id: number;
};

type Instructor = {
  name: string;
  instructor_id: number;
  user_id: number;
};

type Student = {
  full_name: string;
  semester: Semester;
  student_id: number;
  user_id: number;
};


export type Course ={
  course_name: string;
  course_id: number;
  instructors: Instructor[];
  students: Student[];
  features: CourseFeature[];
};

export type CourseResponse = Course[];

export type CreateCourseData = {
  course_name: string;
  student_ids: number[];
  instructor_ids: number[];
  feature_ids: number[];
};

