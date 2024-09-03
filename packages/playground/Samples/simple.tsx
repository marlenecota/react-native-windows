/**
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 * @format
 */

import React from 'react';
import {
  StyleSheet,
  View,
  Text,
  AppRegistry,
  useColorScheme,
  Appearance,
  Button,
  Switch
} from 'react-native';

export const SimpleModule = () => {
  const colorScheme = useColorScheme();

  return (
    <View style={styles.container}>
      <Text>useColorScheme(): {colorScheme}</Text>
      <Button
        title="change color scheme"
        onPress={() => colorScheme === 'dark' ? Appearance.setColorScheme('light') : Appearance.setColorScheme('dark')}
      />
      <Switch />
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
});

AppRegistry.registerComponent('Bootstrap', () => SimpleModule);
