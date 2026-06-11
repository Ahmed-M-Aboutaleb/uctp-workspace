// ServerActions/Enrollment/Actions.ts
'use server'

import { getToken } from "@/cookies/auth.actions";
import { revalidatePath } from "next/cache";

export async function UnEnrollStudent(course_id: number): Promise<boolean> {
  const token = await getToken();
  
  try {
    const response = await fetch(`${process.env.API_URL}/api/enrollment/${course_id}/leave`, {
      method: "DELETE",
      headers: {
        "Authorization": `Bearer ${token}`,
      },
    });

    if (!response.ok) throw new Error(`API error: ${response.status}`);

    revalidatePath("/enroll"); 
    return true;
  } catch (error) {
    console.error("Error Unenrolling:", error);
    throw new Error("Failed to unenroll");
  }
}