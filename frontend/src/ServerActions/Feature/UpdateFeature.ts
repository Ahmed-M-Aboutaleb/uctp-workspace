'use server'

import { getToken } from "@/cookies/auth.actions";
import { revalidatePath } from "next/cache";

export async function UpdateFeature(feature_id: number, feature_name: string): Promise<any> {
    const token = await getToken();
    try {
    const response = await fetch(`${process.env.API_URL}/api/facilities/features/${feature_id}`, {
        method: "PATCH",
        headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/json',
            'Authorization': `Bearer ${token}`
        },
        body: JSON.stringify({ feature_name, feature_id })
    });

    if (!response.ok) {
        throw new Error("Failed to update feature")
    }
    const data = await response.json();
        revalidatePath("/admin");
        revalidatePath("/instructor");
        revalidatePath("/student");
    return data;
    
  } catch (error) {
    console.error("Error updating feature:", error)
    throw error
}
}