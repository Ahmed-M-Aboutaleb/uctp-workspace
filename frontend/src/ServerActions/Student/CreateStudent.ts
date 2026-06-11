'use server'

import { getToken } from "@/cookies/auth.actions";
import { CreateStudentData, CreateStudentResponse } from "@/Types/StudentTypes";
import { revalidatePath } from "next/cache";


export async function CreateStudent(student: CreateStudentData): Promise<CreateStudentResponse> {
    const token = await getToken();
    
    try {
    const response = await fetch(`${process.env.API_URL}/api/people/students`, {
        method: "POST",
        headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
        "Content-Type": 'application/json'
        },
    body: JSON.stringify( student )
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
    console.error(" Error creating student:", error)
    throw new Error("Failed to create student")
    }
}
