'use server';
import { getToken } from "@/cookies/auth.actions";
import { revalidatePath } from "next/cache";

export async function EnrollStudent(course_id: number): Promise<any> {
  const token = await getToken();
  
  try {
    const response = await fetch(`${process.env.API_URL}/api/enrollment/${course_id}/join`, {
      method: "POST",
      headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
      },
      body: JSON.stringify({})
    });

    if (!response.ok) throw new Error(`API error: ${response.status}`);

    const data = await response.json();
    
    revalidatePath("/enroll");
    return data;
  } catch (error) {
    console.error("Error Enrolling:", error);
    throw new Error("Failed to enroll");
  }
}