'use server'

import { getToken } from "@/cookies/auth.actions";
import { CreateFeatureResponse } from "@/Types/FeaturesType";
import { revalidatePath } from "next/cache";


export async function CreateFeature(feature_name:string): Promise<CreateFeatureResponse | { error: string } | Response> {
  const token = await getToken();
  
  try {
    const response = await fetch(`${process.env.API_URL}/api/facilities/features`, {
      method: "POST",
      headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
        "Content-Type": 'application/json'
      },
    body: JSON.stringify({ feature_name })
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
    console.error(" Error creating feature:", error)
    return Response.json(
      { error: "Failed to create feature" },
      { status: 500 }
    )
  }
}
