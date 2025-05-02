<template>
  <div class="juce-toggle">
    <!-- Value text above -->
    <span class="value-text">
      {{ modelValue ? "On" : "Off" }}
    </span>

    <!-- Toggle graphic -->
    <div
      v-if="toggleImage"
      class="toggle-sprite"
      :style="{
        backgroundImage: `url('${toggleImage}')`,
        backgroundPosition: `0px ${modelValue ? `-${frameHeight}px` : '0px'}`,
        backgroundSize: `100% auto`,
      }"
      @click="toggle"
    ></div>

    <!-- Fallback checkbox -->
    <input
      v-else
      type="checkbox"
      :checked="modelValue"
      @change="onChange"
      class="toggle-input"
    />

    <!-- Label text under -->
    <span class="label-text">
      {{ label }}
    </span>
  </div>
</template>

<script setup lang="ts">
const props = defineProps<{
  label: string;
  modelValue: boolean;
  toggleImage?: string;
}>();

const emit = defineEmits<{
  (e: "update:modelValue", value: boolean): void;
}>();

const frameHeight = 50;

function onChange(e: Event) {
  const target = e.target as HTMLInputElement;
  emit("update:modelValue", target.checked);
}

function toggle() {
  emit("update:modelValue", !props.modelValue);
}
</script>

<style scoped>
.juce-toggle {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8px;
}

/* Toggle background sprite */
.toggle-sprite {
  width: 50px; /* one frame width */
  height: 50px; /* one frame height */
  background-repeat: no-repeat;
  background-size: 100% auto;
  cursor: pointer;
}

/* Fallback checkbox input */
.toggle-input {
  width: 24px;
  height: 24px;
  cursor: pointer;
}

/* Value text above toggle */
.value-text {
  font-size: 0.875rem;
  font-weight: 600;
  color: limegreen;
  text-shadow: 0 0 2px black, 0 0 2px black;
  text-align: center;
}

/* Label text under toggle */
.label-text {
  width: 50px;
  height: 30px;
  font-size: 0.75rem;
  font-weight: 500;
  color: limegreen;
  text-align: center;
}
</style>
