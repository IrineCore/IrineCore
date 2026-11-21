USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ReturnQueue](
	[WishListID] [int] NULL,
	[CharacterID] [bigint] NOT NULL,
	[RegistedDate] [datetime2](0) NOT NULL,
	[PushTypeCode] [tinyint] NULL,
	[ItemID] [int] NULL,
	[QueueID] [bigint] IDENTITY(1,1) NOT NULL,
	[AccountID] [int] NULL
) ON [DataFG1]
GO
