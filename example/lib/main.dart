import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'dart:io';
import 'dart:typed_data';

import 'package:flutter/material.dart';

import 'package:flutter/services.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  Uint8List? uint8list;
  Uint8List? blurUint8list;
  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance!.addPostFrameCallback((_) async {
      final bytes = await rootBundle.load('assets/1.jpg');
      uint8list = bytes.buffer.asUint8List();
      blurUint8list = blur(uint8list!);
      setState(() {});
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        body: SafeArea(
          child: ListView(
            shrinkWrap: true,
            children: [
              uint8list == null? Container() : Image.memory(uint8list!),
              blurUint8list == null? Container() : Image.memory(blurUint8list!),
            ],
          ),
        ),
      )
    );
  }

  static Uint8List? blur(Uint8List list) {
    /// 深拷贝图片
    Pointer<Uint8> bytes = malloc.allocate<Uint8>(list.length);
    for (int i = 0; i < list.length; i++) {
      bytes.elementAt(i).value = list[i];
    }
    // 为图片长度分配内存
    final imgLengthBytes = malloc.allocate<Int32>(1)..value = list.length;

    // 查找 C++ 中的 opencv_blur() 函数
    final DynamicLibrary _opencvLib =
    Platform.isAndroid ? DynamicLibrary.open("libnative-lib.so") : DynamicLibrary.process();
    final Pointer<Uint8> Function(
        Pointer<Uint8> bytes, Pointer<Int32> imgLengthBytes, int kernelSize) blur =
    _opencvLib
        .lookup<
        NativeFunction<
            Pointer<Uint8> Function(Pointer<Uint8> bytes, Pointer<Int32> imgLengthBytes,
                Int32 kernelSize)>>("opencv_blur")
        .asFunction();

    /// 调用高斯模糊
    final newBytes = blur(bytes, imgLengthBytes, 15);
    if (newBytes == nullptr) {
      print('高斯模糊失败');
      return null;
    }

    var newList = newBytes.asTypedList(imgLengthBytes.value);

    /// 释放指针
    malloc.free(bytes);
    malloc.free(imgLengthBytes);
    return newList;
  }
}
