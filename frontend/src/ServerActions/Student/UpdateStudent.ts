'use server'

import { getToken } from "@/cookies/auth.actions";
import { CreateStudentResponse, UpdateStudentData } from "@/Types/StudentTypes";
import { revalidatePath } from "next/cache";

export async function UpdateStudent(student_id: number, student_data: UpdateStudentData): Promise<CreateStudentResponse> {
    const token = await getToken();
    try {
    const response = await fetch(`${process.env.API_URL}/api/people/students/${student_id}`, {
        method: "PATCH",
        headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/json',
            'Authorization': `Bearer ${token}`
        },
        body: JSON.stringify(student_data)
    });

    if (!response.ok) {
        throw new Error("Failed to update student")
    }
    const data = await response.json();
        revalidatePath("/admin");
        revalidatePath("/instructor");
        revalidatePath("/student");
    
    return data;

    
  } catch (error) {
    console.error("Error updating student:", error)
    throw error
}
}