USE [dnworld]
GO
/****** Object:  Table [dbo].[TotalPvPRoomMembers]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TotalPvPRoomMembers](
	[TotalPvPRoomID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[ObserverFlag] [bit] NOT NULL,
 CONSTRAINT [PK_TotalPvPRoomMembers] PRIMARY KEY CLUSTERED 
(
	[TotalPvPRoomID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_TotalPvPRoomMembers_ObserverFlag]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_TotalPvPRoomMembers_ObserverFlag] ON [dbo].[TotalPvPRoomMembers]
(
	[ObserverFlag] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[TotalPvPRoomMembers]  WITH CHECK ADD  CONSTRAINT [FK_TotalPvPRooms_TotalPvPRoomMembers_TotalPvPRoomID] FOREIGN KEY([TotalPvPRoomID])
REFERENCES [dbo].[TotalPvPRooms] ([TotalPvPRoomID])
GO
ALTER TABLE [dbo].[TotalPvPRoomMembers] CHECK CONSTRAINT [FK_TotalPvPRooms_TotalPvPRoomMembers_TotalPvPRoomID]
GO
