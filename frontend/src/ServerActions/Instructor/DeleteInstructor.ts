'use server'

import { getToken } from "@/cookies/auth.actions";
import { revalidatePath } from "next/cache";

export async function DeleteInstructor(instructorId: number): Promise<boolean> {
    const token = await getToken();
    try {
    const response = await fetch(`${process.env.API_URL}/api/people/instructors/${instructorId}`, {
      method: "DELETE",
        headers: {
            'Authorization': `Bearer ${token}`,
        }
    });

    if (!response.ok) {
        throw new Error("Failed to delete instructor");
    }
    revalidatePath("/admin");
    revalidatePath("/instructor");
    revalidatePath("/student");

    return true ;
  } catch (error) {
    console.error("Error deleting instructor:", error)
    throw error
  }
}
