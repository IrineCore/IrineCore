USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SaveItemLocationIndex2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-08-21
description : ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = Maxdop ?? ?? by ??? at 2012-01-16
**/

CREATE PROCEDURE [dbo].[P_SaveItemLocationIndex2]
	  @inbCharacterID bigint --// ??? ID
	, @vchItemLocationIndexes varchar(8000) --// (???? & ??) ,? ?????? ??? ?? ??? ???
	, @vchItemSerials varchar(8000) --// (???? & ??) ,? ?????? ??? ?? ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblTemp table (
	OwnerCharacterID bigint NOT NULL,
	ItemSerial bigint NOT NULL,
	ItemLocationIndex tinyint NOT NULL
	PRIMARY KEY (OwnerCharacterID, ItemSerial)
);
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblTemp (OwnerCharacterID, ItemSerial, ItemLocationIndex)
	SELECT @inbCharacterID, CAST(B.string as bigint), CAST(A.string as tinyint) 
	FROM dbo.TF_Split(@vchItemLocationIndexes,',') A 
		INNER JOIN dbo.TF_Split(@vchItemSerials,',') B ON A.seq = B.seq;

	/**_# [ItemLocationIndexes] ???? UPSERT???.*/
	WITH CTE AS
	(
		SELECT ItemSerial, OwnerCharacterID, ItemLocationIndex
		FROM dbo.ItemLocationIndexes WITH(NOLOCK,INDEX(PK_ItemLocationIndexes),FORCESEEK )
		WHERE OwnerCharacterID = @inbCharacterID
	)
	MERGE CTE T
	USING @tblTemp S ON T.ItemSerial = S.ItemSerial
	WHEN MATCHED THEN
		UPDATE SET T.ItemLocationIndex = S.ItemLocationIndex
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (ItemSerial, OwnerCharacterID, ItemLocationIndex)
		VALUES (S.ItemSerial, S.OwnerCharacterID, S.ItemLocationIndex)
	WHEN NOT MATCHED BY SOURCE THEN
		DELETE
	OPTION(MAXDOP 1, MERGE JOIN, KEEPFIXED PLAN);

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
