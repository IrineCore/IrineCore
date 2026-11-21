USE [dnworld]
GO
/****** Object:  Table [dbo].[StageStartIDs]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[StageStartIDs](
	[StageStartID] [bigint] IDENTITY(1,1) NOT NULL,
	[RoomID] [bigint] NOT NULL,
	[PartySize] [tinyint] NOT NULL,
	[MapID] [int] NOT NULL,
	[DifficultyCode] [tinyint] NOT NULL,
	[PartyStartID] [bigint] NULL,
	[StageStartDate] [datetime2](0) NOT NULL,
	[StageEndDate] [datetime2](0) NULL,
	[WhereToGoCode] [tinyint] NULL,
 CONSTRAINT [PK_StageStartIDs] PRIMARY KEY CLUSTERED 
(
	[StageStartID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_StageStartIDs_StageStartDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_StageStartIDs_StageStartDate] ON [dbo].[StageStartIDs]
(
	[StageStartDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_StageStartIDs_RoomID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_StageStartIDs_RoomID] ON [dbo].[StageStartIDs]
(
	[RoomID] ASC
)
INCLUDE([StageStartDate]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
