'use server'

import { getToken } from "@/cookies/auth.actions";
import { Course, CreateCourseData } from "@/Types/CourseTypes";
import { revalidatePath } from "next/cache";

export async function UpdateCourse(course_id: number, updateCourseData: CreateCourseData): Promise<Course> {
    const token = await getToken();
    try {
    const response = await fetch(`${process.env.API_URL}/api/courses/${course_id}`, {
        method: "PATCH",
        headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/json',
            'Authorization': `Bearer ${token}`
        },
        body: JSON.stringify(updateCourseData)
    });

    if (!response.ok) {
        throw new Error("Failed to update course")
    }
    const data = await response.json();
        revalidatePath("/admin");
        revalidatePath("/instructor");
        revalidatePath("/student");
    return data;
    
  } catch (error) {
    console.error("Error updating course:", error)
    throw error
}
}