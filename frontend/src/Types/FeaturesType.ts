export type CreateFeatureResponse = {
  feature_id: number;
  feature_name: string;
}
export interface Feature {
  feature_id: number
  feature_name: string
}

export interface FeatureFormData {
  feature_name: string
}
