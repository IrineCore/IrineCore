USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_MergeDimWorld]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-06-08
description : DimWorlds ???? ?? ??? MERGE???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2	WorldId -> WorldID? ?? (?/??? ??)
**/

CREATE PROCEDURE [dbo].[P_MergeDimWorld]
	  @vchWorldIDs varchar(max) --// ,? ?????? ?? ID ???
	, @nvcWorldNames nvarchar(max) --// ,? ?????? ?? ?? ???
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblWorlds table (WorldID int NOT NULL, WorldName nvarchar(50) NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	MERGE dbo.DimWorlds AS T
	USING (
		SELECT CAST(A.string AS int) AS WorldID, CAST(B.string AS nvarchar(50)) AS WorldName
		FROM dbo.IF_Split(@vchWorldIDs, ',') A
			INNER JOIN dbo.IF_NSplit(@nvcWorldNames, N',') B ON A.seq = B.seq
		WHERE A.string > '' AND B.string > N''
	) AS S
	ON T.WorldID = S.WorldID
	WHEN MATCHED THEN
		UPDATE SET WorldName = S.WorldName
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (WorldID, WorldName) VALUES (S.WorldID, WorldName)
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
