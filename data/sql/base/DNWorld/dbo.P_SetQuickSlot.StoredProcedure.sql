USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SetQuickSlot]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-13
description : ? ??? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
7 = Dead Lock? ???? ?? NOLOCK ?? ?? at 2010-12-20
8 = ObjectValue ??? ??? ??? int -> bigint? ?? at 2011-02-15
9 = Tuning. Split ?? ??     by 20110624.wgkim
9 = Tuning ? ?? ??. identity ? identity(0,1)     by 20110629.wgkim
11 = ?? ?? by ??? at 2011-11-02
12 = QuickSlotIndex ?? 200??? ???? ??? ?? at 2012-07-30 by ???
13 = 12? ?? at 2012-07-30 by ???
**/

CREATE PROCEDURE [dbo].[P_SetQuickSlot]
	  @inbCharacterID bigint --// ??? ID
	, @vchObjectTypeCodes varchar(8000) --// ,? ?????? ?? ?? ?? ?? ?? ???
	, @vchObjectValues varchar(8000) --// ,? ?????? ?? ?? ?? ? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE @intIndex1 int
	  , @intIndex2 int
	  , @inyObjectTypeCode tinyint
	  , @inbObjectValue bigint;

DECLARE @tblQuickSlots table (QuickSlotIndex int identity(0,1), ObjectTypeCode tinyint NOT NULL, ObjectValue bigint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	WHILE (LEN(@vchObjectTypeCodes) > 0)
	BEGIN

		SELECT @intIndex1 = CHARINDEX(',', @vchObjectTypeCodes);
		SELECT @intIndex2 = CHARINDEX(',', @vchObjectValues);

		IF (@intIndex1 = 0) AND (LEN(@vchObjectTypeCodes) > 0)
		BEGIN
			SELECT @inyObjectTypeCode = @vchObjectTypeCodes;
			SELECT @inbObjectValue = @vchObjectValues;

			INSERT @tblQuickSlots (ObjectTypeCode, ObjectValue) SELECT @inyObjectTypeCode, @inbObjectValue;

			BREAK;
		END

		IF (@intIndex1 > 1)
		BEGIN
			SELECT @inyObjectTypeCode = LEFT(@vchObjectTypeCodes, @intIndex1 - 1);
			SELECT @inbObjectValue = LEFT(@vchObjectValues, @intIndex2 - 1);

			INSERT @tblQuickSlots (ObjectTypeCode, ObjectValue) SELECT @inyObjectTypeCode, @inbObjectValue;

			SELECT @vchObjectTypeCodes = RIGHT(@vchObjectTypeCodes, LEN(@vchObjectTypeCodes) - @intIndex1);
			SELECT @vchObjectValues = RIGHT(@vchObjectValues, LEN(@vchObjectValues) - @intIndex2);
		END
		ELSE
		BEGIN
			SELECT @vchObjectTypeCodes = RIGHT(@vchObjectTypeCodes, LEN(@vchObjectTypeCodes) - @intIndex1);
			SELECT @vchObjectValues = RIGHT(@vchObjectValues, LEN(@vchObjectValues) - @intIndex2);
		END

	END;

	/**_# [QuickSlots]???? MERGE???.*/
	WITH C1 AS (
		SELECT CharacterID, QuickSlotIndex, ObjectTypeCode, ObjectValue
		FROM dbo.QuickSlots WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID
	)
	MERGE C1 AS T
	USING @tblQuickSlots AS S
	ON T.QuickSlotIndex = S.QuickSlotIndex
	WHEN MATCHED AND (T.ObjectTypeCode <> S.ObjectTypeCode OR T.ObjectValue <> S.ObjectValue) THEN
		UPDATE SET ObjectTypeCode = S.ObjectTypeCode, ObjectValue = S.ObjectValue
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (CharacterID, QuickSlotIndex, ObjectTypeCode, ObjectValue) VALUES (@inbCharacterID, S.QuickSlotIndex, S.ObjectTypeCode, S.ObjectValue)
	WHEN NOT MATCHED BY SOURCE THEN
		DELETE;
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
