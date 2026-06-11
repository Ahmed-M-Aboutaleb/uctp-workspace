'use server'

import { getToken } from "@/cookies/auth.actions";
import { AdminResponse } from "@/Types/AdminTypes";


export async function GetAdmins(skip: number = 0, limit: number = 100): Promise<AdminResponse[]> {
  const token = await getToken();
  try {
    const response = await fetch(`${process.env.API_URL}/api/people/admins?skip=${skip}&limit=${limit}`, {
      method: "GET",
      headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
      },
    })


    return await response.json();
  } catch (error) {
    console.error(" Error fetching admins from backend:", error)
    throw error
  }
}
