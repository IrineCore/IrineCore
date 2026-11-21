USE [dnworld]
GO
/****** Object:  Table [dbo].[NestClearRecord]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NestClearRecord](
	[MapID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[PartyID] [bigint] NULL,
	[ClearTime] [int] NOT NULL,
	[ClearDate] [datetime] NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_NestClearRecord_ClearTime]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_NestClearRecord_ClearTime] ON [dbo].[NestClearRecord]
(
	[ClearTime] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_NestClearRecord_MapID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_NestClearRecord_MapID] ON [dbo].[NestClearRecord]
(
	[MapID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
