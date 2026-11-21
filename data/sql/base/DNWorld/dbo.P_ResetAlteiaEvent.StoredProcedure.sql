USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ResetAlteiaEvent]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-11-01
description : ???? ???? ??? ???(??? ?? ???? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_ResetAlteiaEvent]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @nvcSQL nvarchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END
BEGIN TRY
	IF OBJECT_ID('VIX_AlteiaReceiveTicketTotals') IS NOT NULL
		DROP VIEW dbo.VIX_AlteiaReceiveTicketTotals; 
	IF OBJECT_ID('VIX_AlteiaSendTicketTotals') IS NOT NULL
		DROP VIEW dbo.VIX_AlteiaSendTicketTotals;

	TRUNCATE TABLE dbo.AlteiaSchedules;
	TRUNCATE TABLE dbo.PlayAlteia;
	TRUNCATE TABLE dbo.AlteiaSendTickets;
	TRUNCATE TABLE dbo.AlteiaGuildRank;

	SET @nvcSQL = N'CREATE VIEW dbo.VIX_AlteiaReceiveTicketTotals 
					WITH SCHEMABINDING AS
						SELECT ReceiveCharacterID AS CharacterID, COUNT_BIG(*) AS ReceiveTicketCount
						FROM dbo.AlteiaSendTickets 
						GROUP BY ReceiveCharacterID;'

	EXEC(@nvcSQL);
	
	SET @nvcSQL = N'CREATE UNIQUE CLUSTERED INDEX IX_UC_VIX_AlteiaReceiveTicketTotals ON dbo.VIX_AlteiaReceiveTicketTotals (CharacterID ASC);';

	EXEC(@nvcSQL);

	SET @nvcSQL = N'CREATE VIEW dbo.VIX_AlteiaSendTicketTotals 
					WITH SCHEMABINDING AS
						SELECT CharacterID, COUNT_BIG(*) AS SendTicketCount
						FROM dbo.AlteiaSendTickets 
						GROUP BY CharacterID;';
	
	EXEC(@nvcSQL);

	SET @nvcSQL = N'CREATE UNIQUE CLUSTERED INDEX IX_UC_VIX_AlteiaSendTicketTotals ON dbo.VIX_AlteiaSendTicketTotals (CharacterID ASC);';
	
	EXEC(@nvcSQL);


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
