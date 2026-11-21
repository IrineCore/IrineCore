USE [dnworld]
GO
/****** Object:  Table [dbo].[Fairies]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Fairies](
	[FairyItemSerial] [bigint] NOT NULL,
	[FairyLevel] [tinyint] NOT NULL,
	[FairyExp] [int] NOT NULL,
	[FairyStatus1] [int] NULL,
	[FairyStatus2] [int] NULL,
	[FairyStatus3] [int] NULL,
	[FairyStatus4] [int] NULL,
	[FairyStatus5] [int] NULL,
	[FairyStatus6] [int] NULL,
	[OwnerCharacterID] [bigint] NULL,
	[ShapeID] [tinyint] NULL,
 CONSTRAINT [PK_Fairies] PRIMARY KEY CLUSTERED 
(
	[FairyItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_Fairies_OwnerCharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Fairies_OwnerCharacterID] ON [dbo].[Fairies]
(
	[OwnerCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
