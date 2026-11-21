USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModPetSkill]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-12
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ????? ?? ? ?? ??? ??? UPDATE ??? ??	by 2011/10/17.ohtack
3 = ?? ?? ?? ??	by 2011/10/17.ohtack
4 = ?? ?? ?? ??	by 2011/10/17.ohtack
5 = ?? ?? ?? ?? ??	by 2011/10/18.ohtack
6 = ?? ?? ?? ??(SP ????)	by 2011/10/18.ohtack
7~8 = ?? ??? 0?? ??? ?? ?? ???? ??	by ??? at 2011-12-12
**/

CREATE PROCEDURE [dbo].[P_ModPetSkill]
	  @inbPetItemSerial bigint --// ?? or ?? ??? ???
    , @intSkillIndex int --// ??? ? ?? ??? (1, 2)
	, @intSkillID int --// ? ?? 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intLogTableSuffix int
	, @inbCharacterID bigint
	, @dt2LogDate datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [PetEquipments]???? ?? ?? UPDATE ???.*/
	IF @intSkillIndex = 1
		UPDATE dbo.PetEquipments
		SET SkillID1 = @intSkillID
		WHERE PetItemSerial = @inbPetItemSerial AND PetEquipmentIndex = 255;
	ELSE IF @intSkillIndex = 2
		UPDATE dbo.PetEquipments
		SET SkillID2 = @intSkillID
		WHERE PetItemSerial = @inbPetItemSerial AND PetEquipmentIndex = 255;	

	/**_# ? ?? ??? INSERT ???.*/
	IF @intSkillID > 0
		EXEC dbo.P_AddPetSkillLog
				@inbPetItemSerial,
				@intSkillID;
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
