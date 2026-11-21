USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModPetSkinColor]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-13
description : ?? or ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = input param. @intSkinColor2 ?? at 2011-05-12
**/

CREATE PROCEDURE [dbo].[P_ModPetSkinColor]
	  @inbPetItemSerial bigint --// ?? or ?? ??? ???
	, @intSkinColor int --// ?? 1 (NULL ??? ???? ????.)
	, @intSkinColor2 int = NULL --// ?? 2 (NULL ??? ???? ????.)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	MERGE dbo.PetEquipments T
	USING (
		SELECT @inbPetItemSerial AS PetItemSerial
			, CAST(255 AS tinyint) AS PetEquipmentIndex
			, @intSkinColor AS SkinColor
			, @intSkinColor2 AS SkinColor2
	) S
	ON T.PetItemSerial = S.PetItemSerial AND T.PetEquipmentIndex = S.PetEquipmentIndex
	WHEN MATCHED THEN
		UPDATE SET SkinColor = ISNULL(S.SkinColor, T.SkinColor), SkinColor2 = ISNULL(S.SkinColor2, T.SkinColor2)
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (PetItemSerial, PetEquipmentIndex, PetEquipmentItemSerial, SkinColor, SkinColor2)
		VALUES (S.PetItemSerial, S.PetEquipmentIndex, NULL, @intSkinColor, @intSkinColor2);
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
