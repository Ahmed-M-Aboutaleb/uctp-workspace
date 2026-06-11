'use server'

import { getToken } from "@/cookies/auth.actions";
import { GetInstructorsResponse } from "@/Types/InstructorTypes";


export async function GetInstructors(skip: number = 0, limit: number = 100): Promise<GetInstructorsResponse> {
  const token = await getToken();
  try {
    const response = await fetch(`${process.env.API_URL}/api/people/instructors?skip=${skip}&limit=${limit}`, {
      method: "GET",
      headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
      },
    })


    return await response.json();
  } catch (error) {
    console.error(" Error fetching instructors from backend:", error)
    throw error
  }
}
