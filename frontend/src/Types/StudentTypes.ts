type Semester = "Fall" | "Spring" | "Summer";

export type CreateStudentResponse = {
    full_name: string;
    semester: Semester;
    student_id: number;
    user_id: number
}

export type CreateStudentData = {
    full_name: string;
    semester: Semester;
    email: string;
    password: string
}

export type UpdateStudentData = {
    full_name: string;
    semester: Semester;
}

export interface Student {
    student_id: number
    full_name: string
    semester: Semester
    user_id: number
}

export interface StudentFormData {
    full_name: string
    semester: Semester
    email: string
    password: string
}

export interface StudentRecord {
    full_name: string;
    semester: Semester;
    student_id: number;
    user_id: number;
}
export interface InstructorRecord {
    name: string;
    instructor_id: number;
    user_id: number;
}

export type StudentResponse = StudentRecord[];