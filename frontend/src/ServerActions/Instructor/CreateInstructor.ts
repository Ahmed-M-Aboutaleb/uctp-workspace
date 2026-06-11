'use server'

import { getToken } from "@/cookies/auth.actions";
import { CreateInstructorData, CreateInstructorResponse } from "@/Types/InstructorTypes";
import { revalidatePath } from "next/cache";


export async function CreateInstructor(instructor: CreateInstructorData): Promise<CreateInstructorResponse> {
    const token = await getToken();
    
    try {
    const response = await fetch(`${process.env.API_URL}/api/people/instructors`, {
        method: "POST",
        headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
        "Content-Type": 'application/json'
        },
    body: JSON.stringify( instructor )
    })

    if (!response.ok) {
        throw new Error(`API responded with status ${response.status}`)
    }

    const data = await response.json()
    revalidatePath("/admin");
    revalidatePath("/instructor");
    revalidatePath("/student");
    return data
    } catch (error) {
    console.error(" Error creating instructor:", error)
    throw new Error("Failed to create instructor")
    }
}
