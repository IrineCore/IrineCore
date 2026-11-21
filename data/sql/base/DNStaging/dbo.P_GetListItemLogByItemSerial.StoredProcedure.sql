USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListItemLogByItemSerial]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-25
description : ??? ???? ??? ??? ?????.
result set : LogDate/datetime2, ItemSerial/bigint, ItemID/int, ItemName/nvarchar, ItemLogCode/tinyint, OwnerCharacterIDBefore/bigint, OwnerCharacterNameBefore/nvarchar, OwnerCharacterID/bigint, OwnerCharacterName/nvarchar, ItemCountBefore/smallint, ItemCount/smallint, MapName/nvarchar, QuestName/nvarchar, IP/varchar, ItemLevel/tinyint, SealCount/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, ItemPotential/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = ItemLogFKey1? int? type cast ?? ??? ?? at 2010-06-22
5 = ItemOption, ItemPotential? ????? ?? at 2010-09-06
6 = ?? ?? at 2010-11-01
8 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
**/

CREATE PROCEDURE [dbo].[P_GetListItemLogByItemSerial]
	  @inbItemSerial bigint --// ??? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'ItemLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(MM, -3, GETDATE()), 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(DD, -1, GETDATE()), 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;
		
	SET @nvcStmt = N'
	DECLARE @tblItems table (
		LogDate datetime2(0),
		ItemSerial bigint,
		ItemID int,
		ItemLogCode tinyint,
		OwnerCharacterIDBefore bigint,
		OwnerCharacterID bigint,
		ItemCountBefore smallint,
		ItemCount smallint,
		IP int,
		ItemLevel tinyint,
		SealCount tinyint,
		SoulBoundFlag bit,
		ItemOption tinyint,
		ItemPotential tinyint,
		ItemLogFKey1 bigint
	);
	INSERT @tblItems
	SELECT V.LogDate, V.ItemSerial, V.ItemID, V.ItemLogCode, V.OwnerCharacterIDBefore, V.OwnerCharacterID
		, V.ItemCountBefore, V.ItemCount, V.IP, V.ItemLevel, V.SealCount, V.SoulBoundFlag, V.ItemOption, V.ItemPotential, V.ItemLogFKey1
	FROM (' + @nvcStmtOUT + N') V
	WHERE V.ItemSerial = @inbItemSerial
	ORDER BY V.LogDate;

	SELECT V.LogDate, V.ItemSerial, V.ItemID, DI.ItemName, V.ItemLogCode, V.OwnerCharacterIDBefore, DC2.CharacterName AS OwnerCharacterNameBefore, V.OwnerCharacterID, DC1.CharacterName AS OwnerCharacterName
		, V.ItemCountBefore, V.ItemCount, DM.MapName, DQ.QuestName, dbo.FN_IPNumber2IP(V.IP) AS IP, V.ItemLevel, V.SealCount, V.SoulBoundFlag, V.ItemOption, V.ItemPotential
	FROM @tblItems V
		LEFT OUTER JOIN dbo.DimCharacters DC1 WITH (NOLOCK) ON V.OwnerCharacterID = DC1.CharacterID
		LEFT OUTER JOIN dbo.DimCharacters DC2 WITH (NOLOCK) ON V.OwnerCharacterIDBefore = DC2.CharacterID
		LEFT OUTER JOIN dbo.DimItems DI WITH (NOLOCK) ON V.ItemID = DI.ItemID
		LEFT OUTER JOIN dbo.DimQuests DQ WITH (NOLOCK) ON V.ItemLogFKey1 = DQ.QuestID AND V.ItemLogCode = 3
		LEFT OUTER JOIN dbo.DimMaps DM WITH (NOLOCK) ON V.ItemLogFKey1 = DM.MapID AND V.ItemLogCode BETWEEN 20 AND 23
	ORDER BY V.LogDate;
	';
	EXEC sp_executesql @nvcStmt, N'@inbItemSerial bigint', @inbItemSerial = @inbItemSerial;
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
