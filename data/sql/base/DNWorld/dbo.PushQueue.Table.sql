USE [dnworld]
GO
/****** Object:  Table [dbo].[PushQueue]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PushQueue](
	[WishListID] [int] NULL,
	[CharacterID] [bigint] NOT NULL,
	[RegistedDate] [datetime2](0) NOT NULL,
	[PushTypeCode] [tinyint] NULL,
	[ItemID] [int] NULL,
	[QueueID] [bigint] IDENTITY(1,1) NOT NULL,
	[AccountID] [int] NULL,
 CONSTRAINT [PK_PushQueue] PRIMARY KEY CLUSTERED 
(
	[QueueID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_PushQueue_WishListID_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PushQueue_WishListID_CharacterID] ON [dbo].[PushQueue]
(
	[WishListID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
