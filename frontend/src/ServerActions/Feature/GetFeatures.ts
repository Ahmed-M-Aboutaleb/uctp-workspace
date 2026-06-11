'use server'

import { getToken } from "@/cookies/auth.actions";


export async function GetFeatures(skip: number = 0, limit: number = 100): Promise<any> {
  const token = await getToken();
  try {
    const response = await fetch(`${process.env.API_URL}/api/facilities/features?skip=${skip}&limit=${limit}`, {
      method: "GET",
      headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
      },
    })


    const data = await response.json()
    
    return data
  } catch (error) {
    console.error(" Error fetching features from backend:", error)
    return Response.json(
      { error: "Failed to fetch features" },
      { status: 500 }
    )
  }
}
