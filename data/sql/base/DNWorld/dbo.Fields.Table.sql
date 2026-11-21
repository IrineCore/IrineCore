USE [dnworld]
GO
/****** Object:  Table [dbo].[Fields]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Fields](
	[FarmID] [int] NOT NULL,
	[FieldIndex] [smallint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[PlantDate] [datetime2](0) NOT NULL,
	[ElapsedTime] [int] NOT NULL,
 CONSTRAINT [PK_Fields] PRIMARY KEY CLUSTERED 
(
	[FarmID] ASC,
	[FieldIndex] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_Fields_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Fields_CharacterID] ON [dbo].[Fields]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[Fields]  WITH CHECK ADD  CONSTRAINT [FK_Farms_Fields_FarmID] FOREIGN KEY([FarmID])
REFERENCES [dbo].[Farms] ([FarmID])
GO
ALTER TABLE [dbo].[Fields] CHECK CONSTRAINT [FK_Farms_Fields_FarmID]
GO
