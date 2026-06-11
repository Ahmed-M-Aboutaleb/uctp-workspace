'use server'

import { getToken } from "@/cookies/auth.actions";
import { InstructorResponse } from "@/Types/InstructorTypes";
import { revalidatePath } from "next/cache";

export async function UpdateInstructor(instructor_id: number, name: string): Promise<InstructorResponse> {
    const token = await getToken();
    try {
    const response = await fetch(`${process.env.API_URL}/api/people/instructors/${instructor_id}`, {
        method: "PATCH",
        headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/json',
            'Authorization': `Bearer ${token}`
        },
        body: JSON.stringify({ name })
    });

    if (!response.ok) {
        throw new Error("Failed to update instructor")
    }
    const data = await response.json();
        revalidatePath("/admin");
        revalidatePath("/instructor");
        revalidatePath("/student");
    
    return data;
    
  } catch (error) {
    console.error("Error updating instructor:", error)
    throw error
}
}