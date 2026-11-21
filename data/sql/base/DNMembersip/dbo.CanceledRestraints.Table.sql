USE [dnmembership]
GO
/****** Object:  Table [dbo].[CanceledRestraints]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CanceledRestraints]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CanceledRestraints](
	[RestraintID] [int] NOT NULL,
	[AdminID] [int] NOT NULL,
	[Memo] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CancelDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_CanceledRestraints] PRIMARY KEY CLUSTERED 
(
	[RestraintID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Restraints_CanceledRestraints_RestraintID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CanceledRestraints]'))
ALTER TABLE [dbo].[CanceledRestraints]  WITH CHECK ADD  CONSTRAINT [FK_Restraints_CanceledRestraints_RestraintID] FOREIGN KEY([RestraintID])
REFERENCES [dbo].[Restraints] ([RestraintID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Restraints_CanceledRestraints_RestraintID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CanceledRestraints]'))
ALTER TABLE [dbo].[CanceledRestraints] CHECK CONSTRAINT [FK_Restraints_CanceledRestraints_RestraintID]
GO
