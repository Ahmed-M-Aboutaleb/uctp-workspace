import { getToken } from "@/cookies/auth.actions";

export async function GetProfile(): Promise<any> {
  const token = await getToken();
  
  if (!token) return null; 
  try {
    const response = await fetch(`${process.env.API_URL}/api/people/me`, {
      method: "GET",
      headers: {
        "Accept": "application/json",
        "Authorization": `Bearer ${token}`,
      },
      cache: "no-store" 
    });

    return await response.json();
  } catch (error) {
    return null;
  }
}